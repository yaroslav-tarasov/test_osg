#pragma once

namespace meta
{
   // из-за того, что данный код будет компилироваться  VC7.0,
   // обойдемся без частичной специализации
   namespace details
   {
      template < bool condition >
         struct _if
      {
         template < typename true_result, typename false_result >
            struct _if_impl
         {
            typedef
               true_result
               type;
         };
      };

      template < >
         struct _if< false >
      {
         template < typename true_result, typename false_result >
            struct _if_impl
         {
            typedef
               false_result
               type;
         };
      };

   }

   template < typename condition, typename true_result, typename false_result >
      struct _if
   {
      typedef
         typename details::_if< condition::value >::template _if_impl< true_result, false_result >::type
         type;
   };

   // ---------------------------------------------------------- integral type function

   template < typename T >
      struct _is_integral
   {
      static const bool value = false;
   };

   template < bool B >
      struct _bool
   {
      static const bool value = B;
   };

   template < > struct _is_integral< short >           { static const bool value = true; };
   template < > struct _is_integral< unsigned short >  { static const bool value = true; };

   template < > struct _is_integral< char >           { static const bool value = true; };
   template < > struct _is_integral< unsigned char >  { static const bool value = true; };

   template < > struct _is_integral< int >           { static const bool value = true; };
   template < > struct _is_integral< unsigned int >  { static const bool value = true; };

   template < > struct _is_integral< long >           { static const bool value = true; };
   template < > struct _is_integral< unsigned long >  { static const bool value = true; };
}
