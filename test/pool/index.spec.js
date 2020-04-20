describe("Pool: Acquire multiple / release all", require("./pool.01")) &&
    describe("Pool: Acquire multiple / release 1", require("./pool.02.1")) &&
    describe("Pool: Acquire 3 / release 3", require("./pool.02.2")) &&
    describe(
        "Pool: Acquire multiple / release multiple",
        require("./pool.03")
    ) &&
    describe("Pool: Error - Release without client", require("./pool.05")) &&
    describe("Pool: Acquire 1 / Release 1", require("./pool.06")) &&
    describe("Pool Options", require("./pool.options"));
