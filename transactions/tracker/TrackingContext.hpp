#pragma once
#include "ObjectBuilder.hpp"

namespace myria { namespace tracker {

		class Tracker;
		
		struct TrackingContext{
			struct Internals;
			Internals *i{nullptr};
			Tracker &trk;
			mutils::abs_StructBuilder& logger;
			TrackingContext(mutils::abs_StructBuilder &logger, Tracker& t, bool commitOnDelete = false);
			void commitContext();
			void abortContext();
			virtual ~TrackingContext();
			TrackingContext(const TrackingContext&) = delete;
			TrackingContext(TrackingContext&&) = delete;
			friend class Tracker;
		};
	}}
