#pragma once

namespace myria { namespace tracker {

		class Tracker;
		
		struct TrackingContext{
			struct Internals;
			Internals *i{nullptr};
			TrackingContext(Tracker& t, bool commitOnDelete = false);
			void commitContext();
			void abortContext();
			virtual ~TrackingContext();
			TrackingContext(const TrackingContext&) = delete;
			TrackingContext(TrackingContext&&) = delete;
			friend class Tracker;
		};
	}}
