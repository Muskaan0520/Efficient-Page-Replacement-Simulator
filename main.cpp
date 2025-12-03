#include <iostream>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

struct Result {
    string name;
    int hits;
    int faults;
    double hitRatio;
    double faultRatio;
};

void printStepHeader(const string& algoName) {
    cout << "\n---------------------------------------------\n";
    cout << "  " << algoName << " - Step-wise Simulation\n";
    cout << "---------------------------------------------\n";
    cout << left << setw(8) << "Step"
         << setw(8) << "Page"
         << setw(20) << "Frames"
         << setw(10) << "Status"
         << setw(10) << "Replaced"
         << "\n";
    cout << "---------------------------------------------\n";
}

void printStepRow(int step, int page, const vector<int>& frames, bool hit, int replaced) {
    cout << left << setw(8) << step;
    cout << setw(8) << page;

    // Frames printing
    string frameStr = "[";
    for (size_t i = 0; i < frames.size(); ++i) {
        if (frames[i] == -1) frameStr += "_";
        else frameStr += to_string(frames[i]);
        if (i != frames.size() - 1) frameStr += " ";
    }
    frameStr += "]";
    cout << setw(20) << frameStr;

    cout << setw(10) << (hit ? "Hit" : "Fault");

    if (replaced == -1) cout << setw(10) << "-";
    else cout << setw(10) << replaced;

    cout << "\n";
}

Result runFIFO(const vector<int>& ref, int framesCount) {
    string algoName = "FIFO";
    printStepHeader(algoName);

    vector<int> frames(framesCount, -1);
    int pointer = 0; // points to next frame to replace (circular)
    int hits = 0, faults = 0;

    for (size_t i = 0; i < ref.size(); ++i) {
        int page = ref[i];
        bool hit = false;
        int replaced = -1;

        // check if page already in frames
        for (int f : frames) {
            if (f == page) {
                hit = true;
                break;
            }
        }

        if (hit) {
            hits++;
        } else {
            faults++;
            // if there is empty frame, use it
            bool placed = false;
            for (int j = 0; j < framesCount; ++j) {
                if (frames[j] == -1) {
                    frames[j] = page;
                    placed = true;
                    break;
                }
            }
            // otherwise replace using FIFO pointer
            if (!placed) {
                replaced = frames[pointer];
                frames[pointer] = page;
                pointer = (pointer + 1) % framesCount;
            }
        }

        printStepRow(i + 1, page, frames, hit, replaced);
    }

    int n = (int)ref.size();
    Result res;
    res.name = algoName;
    res.hits = hits;
    res.faults = faults;
    res.hitRatio = (double)hits / n;
    res.faultRatio = (double)faults / n;
    return res;
}

Result runLRU(const vector<int>& ref, int framesCount) {
    string algoName = "LRU";
    printStepHeader(algoName);

    vector<int> frames(framesCount, -1);
    vector<int> lastUsed(framesCount, -1); // stores last used step index
    int hits = 0, faults = 0;

    for (size_t i = 0; i < ref.size(); ++i) {
        int page = ref[i];
        bool hit = false;
        int replaced = -1;

        int hitIndex = -1;

        // check hit
        for (int j = 0; j < framesCount; ++j) {
            if (frames[j] == page) {
                hit = true;
                hitIndex = j;
                break;
            }
        }

        if (hit) {
            hits++;
            lastUsed[hitIndex] = (int)i;
        } else {
            faults++;
            // look for empty frame
            int emptyIndex = -1;
            for (int j = 0; j < framesCount; ++j) {
                if (frames[j] == -1) {
                    emptyIndex = j;
                    break;
                }
            }

            if (emptyIndex != -1) {
                frames[emptyIndex] = page;
                lastUsed[emptyIndex] = (int)i;
            } else {
                // LRU replacement
                int lruIndex = 0;
                int minLast = lastUsed[0];
                for (int j = 1; j < framesCount; ++j) {
                    if (lastUsed[j] < minLast) {
                        minLast = lastUsed[j];
                        lruIndex = j;
                    }
                }
                replaced = frames[lruIndex];
                frames[lruIndex] = page;
                lastUsed[lruIndex] = (int)i;
            }
        }

        printStepRow(i + 1, page, frames, hit, replaced);
    }

    int n = (int)ref.size();
    Result res;
    res.name = algoName;
    res.hits = hits;
    res.faults = faults;
    res.hitRatio = (double)hits / n;
    res.faultRatio = (double)faults / n;
    return res;
}

Result runOptimal(const vector<int>& ref, int framesCount) {
    string algoName = "Optimal";
    printStepHeader(algoName);

    vector<int> frames(framesCount, -1);
    int hits = 0, faults = 0;

    for (size_t i = 0; i < ref.size(); ++i) {
        int page = ref[i];
        bool hit = false;
        int replaced = -1;

        // check hit
        for (int f : frames) {
            if (f == page) {
                hit = true;
                break;
            }
        }

        if (hit) {
            hits++;
        } else {
            faults++;
            // empty frame?
            int emptyIndex = -1;
            for (int j = 0; j < framesCount; ++j) {
                if (frames[j] == -1) {
                    emptyIndex = j;
                    break;
                }
            }

            if (emptyIndex != -1) {
                frames[emptyIndex] = page;
            } else {
                // choose page with farthest next use
                int indexToReplace = -1;
                int farthestUse = -1;

                for (int j = 0; j < framesCount; ++j) {
                    int currentPage = frames[j];
                    int nextUse = -1;
                    for (size_t k = i + 1; k < ref.size(); ++k) {
                        if (ref[k] == currentPage) {
                            nextUse = (int)k;
                            break;
                        }
                    }
                    // if not used again, replace this
                    if (nextUse == -1) {
                        indexToReplace = j;
                        break;
                    }
                    if (nextUse > farthestUse) {
                        farthestUse = nextUse;
                        indexToReplace = j;
                    }
                }

                replaced = frames[indexToReplace];
                frames[indexToReplace] = page;
            }
        }

        printStepRow(i + 1, page, frames, hit, replaced);
    }

    int n = (int)ref.size();
    Result res;
    res.name = algoName;
    res.hits = hits;
    res.faults = faults;
    res.hitRatio = (double)hits / n;
    res.faultRatio = (double)faults / n;
    return res;
}

Result runLFU(const vector<int>& ref, int framesCount) {
    string algoName = "LFU";
    printStepHeader(algoName);

    vector<int> frames(framesCount, -1);
    vector<int> freq(framesCount, 0);
    vector<int> loadTime(framesCount, -1); // to break ties
    int hits = 0, faults = 0;

    for (size_t i = 0; i < ref.size(); ++i) {
        int page = ref[i];
        bool hit = false;
        int replaced = -1;
        int hitIndex = -1;

        // check hit
        for (int j = 0; j < framesCount; ++j) {
            if (frames[j] == page) {
                hit = true;
                hitIndex = j;
                break;
            }
        }

        if (hit) {
            hits++;
            freq[hitIndex]++;
        } else {
            faults++;
            int emptyIndex = -1;
            for (int j = 0; j < framesCount; ++j) {
                if (frames[j] == -1) {
                    emptyIndex = j;
                    break;
                }
            }

            if (emptyIndex != -1) {
                frames[emptyIndex] = page;
                freq[emptyIndex] = 1;
                loadTime[emptyIndex] = (int)i;
            } else {
                // find LFU (tie -> oldest loadTime)
                int idx = 0;
                for (int j = 1; j < framesCount; ++j) {
                    if (freq[j] < freq[idx]) {
                        idx = j;
                    } else if (freq[j] == freq[idx]) {
                        if (loadTime[j] < loadTime[idx]) {
                            idx = j;
                        }
                    }
                }
                replaced = frames[idx];
                frames[idx] = page;
                freq[idx] = 1;
                loadTime[idx] = (int)i;
            }
        }

        printStepRow(i + 1, page, frames, hit, replaced);
    }

    int n = (int)ref.size();
    Result res;
    res.name = algoName;
    res.hits = hits;
    res.faults = faults;
    res.hitRatio = (double)hits / n;
    res.faultRatio = (double)faults / n;
    return res;
}

Result runSecondChance(const vector<int>& ref, int framesCount) {
    string algoName = "Second Chance";
    printStepHeader(algoName);

    vector<int> frames(framesCount, -1);
    vector<int> refBit(framesCount, 0);
    int pointer = 0;
    int hits = 0, faults = 0;

    for (size_t i = 0; i < ref.size(); ++i) {
        int page = ref[i];
        bool hit = false;
        int replaced = -1;

        // check hit
        int hitIndex = -1;
        for (int j = 0; j < framesCount; ++j) {
            if (frames[j] == page) {
                hit = true;
                hitIndex = j;
                break;
            }
        }

        if (hit) {
            hits++;
            refBit[hitIndex] = 1; // give second chance
        } else {
            faults++;
            // check empty frame
            int emptyIndex = -1;
            for (int j = 0; j < framesCount; ++j) {
                if (frames[j] == -1) {
                    emptyIndex = j;
                    break;
                }
            }

            if (emptyIndex != -1) {
                frames[emptyIndex] = page;
                refBit[emptyIndex] = 1;
            } else {
                // second chance replacement
                while (true) {
                    if (refBit[pointer] == 0) {
                        replaced = frames[pointer];
                        frames[pointer] = page;
                        refBit[pointer] = 1;
                        pointer = (pointer + 1) % framesCount;
                        break;
                    } else {
                        refBit[pointer] = 0;
                        pointer = (pointer + 1) % framesCount;
                    }
                }
            }
        }

        printStepRow(i + 1, page, frames, hit, replaced);
    }

    int n = (int)ref.size();
    Result res;
    res.name = algoName;
    res.hits = hits;
    res.faults = faults;
    res.hitRatio = (double)hits / n;
    res.faultRatio = (double)faults / n;
    return res;
}

void printResultSummary(const Result& r) {
    cout << "\n=== " << r.name << " SUMMARY ===\n";
    cout << "Total Hits   : " << r.hits << "\n";
    cout << "Total Faults : " << r.faults << "\n";
    cout << fixed << setprecision(3);
    cout << "Hit Ratio    : " << r.hitRatio << "\n";
    cout << "Fault Ratio  : " << r.faultRatio << "\n";
}

int main() {
    int n;
    cout << "Enter length of reference string: ";
    cin >> n;

    vector<int> ref(n);
    cout << "Enter the reference string (space separated page numbers):\n";
    for (int i = 0; i < n; ++i) {
        cin >> ref[i];
    }

    int framesCount;
    cout << "Enter number of frames: ";
    cin >> framesCount;

    int choice;
    cout << "\nChoose an option:\n";
    cout << "1. FIFO\n";
    cout << "2. LRU\n";
    cout << "3. Optimal\n";
    cout << "4. LFU\n";
    cout << "5. Second Chance\n";
    cout << "6. Run All & Compare\n";
    cout << "Enter choice: ";
    cin >> choice;

    cout << "\n=====================================================\n";
    cout << "      Efficient Page Replacement Algorithm Simulator\n";
    cout << "=====================================================\n";

    if (choice == 1) {
        Result r = runFIFO(ref, framesCount);
        printResultSummary(r);
    } else if (choice == 2) {
        Result r = runLRU(ref, framesCount);
        printResultSummary(r);
    } else if (choice == 3) {
        Result r = runOptimal(ref, framesCount);
        printResultSummary(r);
    } else if (choice == 4) {
        Result r = runLFU(ref, framesCount);
        printResultSummary(r);
    } else if (choice == 5) {
        Result r = runSecondChance(ref, framesCount);
        printResultSummary(r);
    } else if (choice == 6) {
        Result r1 = runFIFO(ref, framesCount);
        Result r2 = runLRU(ref, framesCount);
        Result r3 = runOptimal(ref, framesCount);
        Result r4 = runLFU(ref, framesCount);
        Result r5 = runSecondChance(ref, framesCount);

        printResultSummary(r1);
        printResultSummary(r2);
        printResultSummary(r3);
        printResultSummary(r4);
        printResultSummary(r5);

        cout << "\n=========== COMPARISON TABLE ===========\n";
        cout << left << setw(15) << "Algorithm"
             << setw(12) << "Hits"
             << setw(12) << "Faults"
             << setw(12) << "HitRatio"
             << setw(12) << "FaultRatio"
             << "\n";
        cout << "-----------------------------------------------\n";
        vector<Result> all = {r1, r2, r3, r4, r5};
        cout << fixed << setprecision(3);
        for (auto &r : all) {
            cout << setw(15) << r.name
                 << setw(12) << r.hits
                 << setw(12) << r.faults
                 << setw(12) << r.hitRatio
                 << setw(12) << r.faultRatio
                 << "\n";
        }
    } else {
        cout << "Invalid choice.\n";
    }

    return 0;
}
