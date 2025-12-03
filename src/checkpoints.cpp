// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 5000;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    // TestNet has no checkpoints (empty map)
    static MapCheckpoints mapCheckpointsTestnet;

    // Lazily-initialized mainnet checkpoints to avoid static initialization
    // order issues (do not call Params() at global/static init time).
    static MapCheckpoints& MainMapCheckpoints()
    {
        static MapCheckpoints mapCheckpoints;
        static bool inited = false;
        if (!inited) {
            inited = true;
            mapCheckpoints[0] = Params().HashGenesisBlock();
            mapCheckpoints[2] = uint256("0x2cb4de1dbfc14b3b71692f747e07e973f1d58328bf54fb709b27818b7a04f337");
            mapCheckpoints[11] = uint256("0xf3cc6f9b42186759abdf5c7a0716bd9da77681a5c2c88b6a88cb91cfc16350f6");
            mapCheckpoints[90] = uint256("0xdd46ec748365a09494f00c03ea9d6e24956df3adafcfe8f4939cd7b61445748f");
            mapCheckpoints[2700] = uint256("0x0dda29f802f7cee87e84c8e46d4e6b9e24e829b75f8e3d327fd1e354dddb22b2");
        }
        return mapCheckpoints;
    }

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        // Explicitly bypass hardened checkpoints in regtest to avoid
        // static-init/selection edge-cases that can still enforce mainnet
        // checkpoints when running local regression tests.
        if (Params().NetworkID() == CChainParams::REGTEST) return true;

        // For other networks treat testnet as empty checkpoints and mainnet
        // uses the hardcoded map.
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : MainMapCheckpoints());

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : MainMapCheckpoints());

        if (checkpoints.empty())
            return 0;
        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (TestNet() ? mapCheckpointsTestnet : MainMapCheckpoints());

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // Automatically select a suitable sync-checkpoint 
    const CBlockIndex* AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && pindex->nHeight + nCheckpointSpan > pindexBest->nHeight)
            pindex = pindex->pprev;
        return pindex;
    }

    // Check against synchronized checkpoint
    bool CheckSync(int nHeight)
    {
        const CBlockIndex* pindexSync = AutoSelectSyncCheckpoint();
        if (nHeight <= pindexSync->nHeight){
            return false;
        }
        return true;
    }
}
