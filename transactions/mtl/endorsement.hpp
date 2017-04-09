#pragma once

namespace myria { namespace mtl {
    template<typename pre, typename post>
    struct endorse_split_transaction {
      using pre_endorse = pre;
      using post_endorse = post;
    };
  }}
