CREATE TRIGGER trg_intstore_numbering BEFORE UPDATE
ON "IntStore" FOR EACH ROW
WHEN (NEW.index <> 0 AND OLD.data IS DISTINCT FROM NEW.data)
EXECUTE PROCEDURE intstore_numbering();

CREATE OR REPLACE FUNCTION intstore_numbering() RETURNS TRIGGER AS
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

    UPDATE "IntStore"
        SET data = NEW.data,
            vc1 = GREATEST(max.vc1, NEW.vc1),
            vc2 = GREATEST(max.vc2, NEW.vc2),
            vc3 = GREATEST(max.vc3, NEW.vc3),
            vc4 = GREATEST(max.vc4, NEW.vc4)
        FROM (SELECT MAX(b.vc1) AS vc1,
                     MAX(b.vc2) AS vc2,
                     MAX(b.vc3) AS vc3,
                     MAX(b.vc4) AS vc4
              FROM "IntStore" b
              WHERE b.id = NEW.id
                  AND b.index <> 0) AS max
        WHERE id = NEW.id
            AND index = 0;
    RETURN NEW;
END;
$do$ LANGUAGE plpgsql;
