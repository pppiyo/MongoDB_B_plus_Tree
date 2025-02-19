'use strict';

/**
 * rename_capped_collection_droptarget.js
 *
 * Creates a capped collection and then repeatedly executes the renameCollection
 * command against it. Inserts documents into the "to" namespace and specifies
 * dropTarget=true.
 *
 * @tags: [requires_capped]
 */

var $config = (function() {
    var data = {
        // Use the workload name as a prefix for the collection name,
        // since the workload name is assumed to be unique.
        prefix: 'rename_capped_collection_droptarget'
    };

    var states = (function() {
        var options = {capped: true, size: 4096};

        function uniqueCollectionName(prefix, tid, num) {
            return prefix + tid + '_' + num;
        }

        function insert(db, collName, numDocs) {
            for (var i = 0; i < numDocs; ++i) {
                var res = db[collName].insert({});
                assertAlways.commandWorked(res);
                assertAlways.eq(1, res.nInserted);
            }
        }

        function init(db, collName) {
            var num = 0;
            this.fromCollName = uniqueCollectionName(this.prefix, this.tid, num++);
            this.toCollName = uniqueCollectionName(this.prefix, this.tid, num++);

            assertAlways.commandWorked(db.createCollection(this.fromCollName, options));
            assertWhenOwnDB(db[this.fromCollName].isCapped());
        }

        function rename(db, collName) {
            // Clear out the "from" collection and insert 'fromCollCount' documents
            var fromCollCount = 7;
            assertWhenOwnDB(db[this.fromCollName].drop());
            // NamespaceNotFound is an acceptable error since it's possible for the collection to
            // have been dropped before the call to listCollections (issued during the
            // createCollection command).
            assertAlways.commandWorkedOrFailedWithCode(
                db.createCollection(this.fromCollName, options), ErrorCodes.NamespaceNotFound);
            assertWhenOwnDB(db[this.fromCollName].isCapped());
            insert(db, this.fromCollName, fromCollCount);

            var toCollCount = 4;
            // NamespaceNotFound is an acceptable error since it's possible for the collection to
            // have been dropped before the call to listCollections (issued during the
            // createCollection command).
            assertAlways.commandWorkedOrFailedWithCode(
                db.createCollection(this.toCollName, options), ErrorCodes.NamespaceNotFound);
            insert(db, this.toCollName, toCollCount);

            // Verify that 'fromCollCount' documents exist in the "to" collection
            // after the rename occurs
            var res =
                db[this.fromCollName].renameCollection(this.toCollName, true /* dropTarget */);

            // SERVER-57128: NamespaceNotFound is an acceptable error if the mongos retries
            // the rename after the coordinator has already fulfilled the original request
            assertWhenOwnDB.commandWorkedOrFailedWithCode(res, ErrorCodes.NamespaceNotFound);

            assertWhenOwnDB(db[this.toCollName].isCapped());
            assertWhenOwnDB.eq(fromCollCount, db[this.toCollName].find().itcount());
            assertWhenOwnDB.eq(0, db[this.fromCollName].find().itcount());

            // Swap "to" and "from" collections for next execution
            var temp = this.fromCollName;
            this.fromCollName = this.toCollName;
            this.toCollName = temp;
        }

        return {init: init, rename: rename};
    })();

    var transitions = {init: {rename: 1}, rename: {rename: 1}};

    return {
        threadCount: 10,
        iterations: 20,
        data: data,
        states: states,
        transitions: transitions,
    };
})();
