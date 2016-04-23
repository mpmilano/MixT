#pragma once
#include "ObjectBuilder.hpp"

namespace myria { namespace tracker {

		class Tracker;
		
		struct TrackingContext{
			struct Internals;
			Internals *i{nullptr};
			Tracker &trk;
			std::unique_ptr<mutils::abs_StructBuilder> logger;
			TrackingContext(std::unique_ptr<mutils::abs_StructBuilder> logger, Tracker& t, bool commitOnDelete = false);
			void commitContext();
			void abortContext();
			virtual ~TrackingContext();
			TrackingContext(const TrackingContext&) = delete;
			TrackingContext(TrackingContext&&) = delete;
			friend class Tracker;
		};
	}}
