#pragma once

namespace cg
{


inline boost::optional<std::pair<matrix_4f,matrix_4f>> clpt_matrix(
    frustum_f const & frustum, float min_farplane,
    range_2f const & layer_z_range )
{
    // make big box around viewer planar pos
    rectangle_3f receiver_box(point_3f(frustum.camera().position().x, frustum.camera().position().y, layer_z_range.center()));
    float const far_fr_dist = cg::norm(frustum.camera_space_frustum_point(FR_right_top_far));
    receiver_box.inflate(point_3f(far_fr_dist, far_fr_dist, 0.5f * layer_z_range.size()));

    // get inter points
    std::vector<point_3f> fInterPoints;
    frustum_rectangle_intersection(frustum, receiver_box, 0.01f, &fInterPoints);
    if (fInterPoints.size() < 3)
        return boost::none;

    std::vector<point_3> aInterPoints;
    for (unsigned i = 0; i < fInterPoints.size(); ++i)
        aInterPoints.emplace_back(fInterPoints[i]);

    // vectors
    const point_3 vViewerPos = frustum.camera().position();
    const point_3 vViewerDir = frustum.camera().dir();
    const point_3 vViewerRight = frustum.camera().right();

    // cast direction is always upward for now
    const point_3 vCastDir(0, 0, 1);

    // get ideal case angular notification
    const double dCosGamma = cg::min(fabs(vViewerDir * vCastDir), 1.0);
    const double dSinGamma = sqrt(1.0 - dCosGamma * dCosGamma);

    cg::matrix_4 mViewMatrix, mProjMatrix;
    // special case - dueling frustums, so orthographic projection needed
    if (eq_zero(dSinGamma, 0.01))
    {
        // special case - orthographic transform
        const cg::point_3 vCastUp = normalized(vCastDir ^ vViewerRight);
        const cg::point_3 vCastSide = normalized(vCastUp ^ vCastDir);
        mViewMatrix = matrix_camera(vViewerPos, vCastDir, vCastUp, vCastSide);

        // get rectangle
        rectangle_3 rcLightViewReceiversBox;
        for (unsigned i = 0; i < aInterPoints.size(); ++i)
            rcLightViewReceiversBox |= mViewMatrix * cg::point_4(aInterPoints[i], 1.0);

        // calc matrices
        mProjMatrix = matrix_ortho(
            rcLightViewReceiversBox.x.lo(), rcLightViewReceiversBox.x.hi(),
            rcLightViewReceiversBox.y.lo(), rcLightViewReceiversBox.y.hi(),
            rcLightViewReceiversBox.z.lo(), rcLightViewReceiversBox.z.hi());
    }
    else
    {
        // usual case - LISPSM-like transform
        const point_3 vCastUp = normalized((vCastDir ^ vViewerDir) ^ vCastDir);
        const point_3 vCastSide = normalized(vCastUp ^ vCastDir);
        mViewMatrix = matrix_camera(vViewerPos, vCastDir, vCastUp, vCastSide);

        // transform points to this space
        rectangle_3 rcAABBLightView;
        for (unsigned i = 0; i < aInterPoints.size(); ++i)
            rcAABBLightView |= mViewMatrix * cg::point_4(aInterPoints[i], 1.0);

        // height over range
        range_2 waveRangeModified = layer_z_range;
        waveRangeModified.inflate(2.5 * layer_z_range.size());
        const double dMaxSignificantHeightOverPlane = cg::max(cg::max(fabs(vViewerPos.z - waveRangeModified.lo()), fabs(vViewerPos.z - waveRangeModified.hi())), waveRangeModified.size());

        // back-offset by depth to match rectangle
        const double m_dNearCam = frustum.clipping_near();
        const double dBackOffset = cg::max(m_dNearCam - rcAABBLightView.y.lo(), 0.0);
        range_2 rangeFwdCorrected = range_2(rcAABBLightView.y.lo() + dBackOffset, rcAABBLightView.y.hi() + dBackOffset);

        // calculate "optimal" projector position
        const double dNopt = dMaxSignificantHeightOverPlane/* * dCosGamma*/ / dSinGamma;

        // get projector position - center and step back from viewer along direction axis
        // add offset because last column is negative dot products
        mViewMatrix(1,3) += dBackOffset + dNopt;

        // calculate frustum rectangle
        rectangle_3 rcFrustum;
        for (unsigned i = 0; i < aInterPoints.size(); ++i)
        {
            const auto v = mViewMatrix * cg::point_4(aInterPoints[i], 1.0);
            Assert(v.y > 0);
            rcFrustum |= point_3(v.x / v.y, v.y, v.z / v.y);
        }

        // deepness into the range
        double dRangeDeepness = cg::clamp(0.0, 2.0 * layer_z_range.size(), 0.0, 1.0)(cg::distance(layer_z_range, vViewerPos.z));
        dRangeDeepness *= dRangeDeepness * (2.0 - dRangeDeepness);
        // change frustum far according to some heuristics
        rcFrustum.y = range_2(rcFrustum.y.lo(), rcFrustum.y.lo() + std::min(rcFrustum.y.size(), lerp01(double(min_farplane), rcFrustum.y.size(), dRangeDeepness)));

        // save projection matrix
        const double
            dNearPlane = cg::max(dNopt, rcFrustum.y.lo()),
            dFarPlane = rcFrustum.y.hi(),
            dRight = rcFrustum.x.hi() * dNearPlane,
            dLeft = rcFrustum.x.lo() * dNearPlane,
            dWidthInv = 1.0 / (dRight - dLeft),
            dTop = rcFrustum.z.hi() * dNearPlane,
            dBottom = rcFrustum.z.lo() * dNearPlane,
            dHeightInv = 1.0 / (dTop - dBottom),
            dFarNearInv = 1.0 / (dFarPlane - dNearPlane);

        double dvProjection[4][4] =
        {
            {dWidthInv * 2.0 * dNearPlane, -dWidthInv * (dLeft + dRight),          0.0,                            0.0},
            {0.0,                          dFarNearInv * (dNearPlane + dFarPlane), 0.0,                            -dFarNearInv * 2.0 * dNearPlane * dFarPlane},
            {0.0,                          dHeightInv * (dTop + dBottom),          -dHeightInv * 2.0 * dNearPlane, 0.0},
            {0.0,                          1.0,                                    0.0,                            0.0},
        };
        mProjMatrix = matrix_4((const double *)dvProjection);
    }

    return std::make_pair(mViewMatrix , mProjMatrix);
}


}