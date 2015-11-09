CREATE TRIGGER trg_update_crap BEFORE UPDATE
ON "BlobStore" FOR EACH ROW
WHEN (NEW.index <> 0 AND OLD.data IS DISTINCT FROM NEW.data)
EXECUTE PROCEDURE update_crap();

CREATE OR REPLACE FUNCTION update_crap() RETURNS TRIGGER AS
$do$
DECLARE
    cntr integer;
BEGIN
    SELECT c.counter
        INTO cntr
        FROM counters c
        WHERE c.index = NEW.lw
	FOR UPDATE ;
    UPDATE counters c
        SET counter = (cntr + 1)
        WHERE c.index = NEW.lw
        RETURNING c.counter
        INTO cntr;

    CASE NEW.lw
        WHEN 1 THEN NEW.vc1 := cntr;
        WHEN 2 THEN NEW.vc2 := cntr;
        WHEN 3 THEN NEW.vc3 := cntr;
        WHEN 4 THEN NEW.vc4 := cntr;
        ELSE RAISE EXCEPTION 'unknown last writer: ' ;
    END CASE;

    UPDATE "BlobStore"
        SET data = NEW.data,
            vc1 = MAX(max.vc1, NEW.v1),
            vc2 = MAX(max.vc2, NEW.v2),
            vc3 = MAX(max.vc3, NEW.v3),
            vc4 = MAX(max.vc4, NEW.v4)
        FROM (SELECT MAX(b.vc1) AS vc1,
                     MAX(b.vc2) AS vc2,
                     MAX(b.vc3) AS vc3,
                     MAX(b.vc4) AS vc4
              FROM "BlobStore" b
              WHERE b."ID" = NEW."ID"
                  AND b.index <> 0) AS max
        WHERE "ID" = NEW."ID"
            AND index = 0;
    RETURN NEW;
END;
$do$ LANGUAGE plpgsql;
