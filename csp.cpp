#define OP_ADD 0
#define OP_SUB 1
#define OP_MUL 2
#define OP_DIV 3
#define OP_NON 4
#define OP_ERR 5

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "string.h"
#include "math.h"

using namespace std;

ofstream ofile;




vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> sting_split(const string &s, char delim) {
    vector<string> elems;
    return split(s, delim, elems);
}


int string_to_int(string & str) {
    return atoi(str.c_str());
}


short op(string & op_str){
    if (op_str == "+") return OP_ADD;
    if (op_str == "-") return OP_SUB;
    if (op_str == "*") return OP_MUL;
    if (op_str == "/") return OP_DIV;
    if (op_str == ".") return OP_NON;
    return OP_ERR;
}

struct Var {
    short psize;
    short x;
    short y;
    short idx;
    short cage_idx;
    set<short> dom;
    
    Var(short idx, short cage_idx, short psize) {
        this->idx = idx;
        this->cage_idx = cage_idx;
        this->psize = psize;
        this->x = idx % psize;
        this->y = idx / psize;
        for(short i=1; i<=psize; ++i) this->dom.insert(i);
    }
};


struct Cage {
    short idx;
    short val;
    short op;
    short psize;
    vector<Var *> vars;
};


struct CSP {
    short psize;
    vector<Var *> vars;
    vector<Cage *> cages;
    Var *** vmap;
    short ** MD_CACHE;
    
    void init_vmap() {
        this->vmap = new Var **[this->psize];
        for(short i=0; i<this->psize; ++i)
            this->vmap[i] = new Var * [this->psize];
        for(short i=0; i<this->vars.size(); ++i)
            this->vmap[this->vars[i]->x][this->vars[i]->y] = this->vars[i];
    }
};


CSP *
init_problem(const char* file_path) {
    ifstream pr_file(file_path);
    string line;
    CSP * csp = new CSP();
    
    vector< vector<string> > raw_problem;
    
    while (pr_file.good()){
        getline(pr_file, line);
        vector<string> tokens = sting_split(line, ' ');
        raw_problem.push_back(tokens);
    }
    pr_file.close();
	
    short vars_size = 0;
    for(short i=0; i<raw_problem.size(); ++i) {
        vars_size += raw_problem[i].size() - 2;
    }
        
    short psize = sqrt(vars_size);
    csp->psize = psize;
    for(short i=0; i<vars_size; ++i)
        csp->vars.push_back(NULL);
    csp->MD_CACHE = new short * [csp->psize];
    for(short i=0; i<csp->psize; ++i)
        csp->MD_CACHE[i] = new short[csp->psize];
    
    
    for(short i=0; i<raw_problem.size(); ++i) {
        Cage * cage = new Cage();
        
        cage->idx = i;
        cage->val = string_to_int(raw_problem[i][0]);
        cage->op = op(raw_problem[i][1]);
        cage->psize = psize;
        
        for(short j=2; j<raw_problem[i].size(); ++j){
            short idx = string_to_int(raw_problem[i][j]);
            Var * var = new Var(idx, cage->idx, psize);
            //cout << endl << idx;
            
            cage->vars.push_back(var);
            csp->vars[idx] = var;
        }
        
        csp->cages.push_back(cage);
    }

    //cout << csp->psize << endl;

    csp->init_vmap();
    return csp;
}


void
print_state(CSP * csp, map<short, short> * assigned) {
    //cout << endl;
    for(short j=0; j<csp->psize; ++j){
        for(short i=0; i<csp->psize; ++i){
            if(assigned->count(csp->vmap[i][j]->idx) == 1){
                ofile << (*assigned)[csp->vmap[i][j]->idx];
            } else {
                ofile << '_';
            }
            ofile << ' ';
        }
        ofile << endl;
    }
    
    ofile << endl;
    
    // for(short j=0; j<csp->psize; ++j){
    //     for(short i=0; i<csp->psize; ++i){
    //         cout << csp->vmap[i][j]->cage_idx << ' ';
    //     }
    //     cout << endl;
    // }
    
}

void prin_raw(CSP * csp){
    for(short i=0; i<csp->vars.size(); ++i)
        cout << csp->vars[i]->idx << ":"
             << csp->vars[i]->cage_idx << endl;
}


// reduces variables domains and returns initial variables assigned
map<short, short> *
reduce_domains(CSP * csp) {
    vector<short> x_redct;
    vector<short> y_redct;
    map<short, short> * assigned = new map<short, short>();
    
    // cout << "Domains reduction:";
    
    for(short i=0; i<csp->cages.size(); ++i) {
        Cage * cage = csp->cages[i];
        if(cage->vars.size() == 1){
            x_redct.push_back(cage->vars[0]->x);
            y_redct.push_back(cage->vars[0]->y);
            assigned->insert(pair<short, short>(cage->vars[0]->idx, cage->val));
        }
    }
    
    
    for(short i=0; i<x_redct.size(); ++i)
        for(short j=0; j<csp->vars.size(); ++j)
            if(csp->vars[j]->x == x_redct[i] && csp->vars[j]->y != y_redct[i]) {
                short assigned_id = csp->vmap[x_redct[i]][y_redct[i]]->idx;
                short assigned_val = (*assigned)[assigned_id];
                csp->vars[j]->dom.erase(assigned_val);
                 // cout << endl << "removed "
                 //     << assigned_val << " from domain of "
                 //     << csp->vars[j]->idx
                 //     << " by unary constraint of "
                 //     << csp->vmap[x_redct[i]][y_redct[i]]->idx; 
            }
            
    for(short i=0; i<y_redct.size(); ++i)
        for(short j=0; j<csp->vars.size(); ++j)
            if(csp->vars[j]->x != x_redct[i] && csp->vars[j]->y == y_redct[i]) {
                short assigned_id = csp->vmap[x_redct[i]][y_redct[i]]->idx;
                short assigned_val = (*assigned)[assigned_id];
                csp->vars[j]->dom.erase(assigned_val);
                 // cout << endl << "removed "
                 //     << assigned_val << " from domain of "
                 //     << csp->vars[j]->idx
                 //     << " by unary constraint of "
                 //     << csp->vmap[x_redct[i]][y_redct[i]]->idx; 
            }
    
    // cout << endl;
    return assigned;
}


set<short>* init_unsaasigned(CSP * csp, map<short, short> * assigned) {
    set<short> * unassigned = new set<short>();
    for(short i=0; i<csp->vars.size(); ++i)
        if(assigned->count(csp->vars[i]->idx) == 0)
            unassigned->insert(csp->vars[i]->idx);
    return unassigned;
}


short
naive_unassigned_selection(set<short> * unassigned) {
    if (unassigned->size() == 0) return -1;
    short selected = *unassigned->begin();
    //unassigned->erase(selected);
    return selected;
}


void
put_back_selected_var(short selected, set<short> * unassigned) {
    unassigned->insert(selected);
}


set<short> *
order_domain_values(short var_idx, map<short, short> * assigned, CSP* csp) {
    return &(csp->vars[var_idx]->dom);
}


bool
consistent(short val, short var_idx, map<short, short> * assigned, CSP * csp) {
    Var * var = csp->vars[var_idx];
    
    // cout << "check consistency for value " << val
    //      << " of variable " << var->idx
    //      << " cage(" << var->cage_idx << ")"
    //      << ": ";
    //     
    // AllDiff
    // iterate over all assigned vars
    for(map<short, short>::const_iterator i=assigned->begin(); i!=assigned->end(); ++i) {
        short asgn_var_id = i->first;
        short asgn_var_val = i->second;
        Var * asgn_var = csp->vars[asgn_var_id];
        // check if assigned var has same coords and test its values;
        if((asgn_var->x == var->x || asgn_var->y == var->y) && asgn_var_val == val){
            // cout << "AllDiff test failed" << endl;
            return false;
        }
    }
    
    // CageTargetVal
    // Get all assigned cage vars.
    // if their number == cages size - 1 then check target val with new assigned var
    vector<short> cage_vals;
    cage_vals.push_back(val);
    Cage * cage = csp->cages[var->cage_idx];
    
    // cout << endl << "cage(" << var->cage_idx << "):";
    for(map<short, short>::const_iterator i=assigned->begin(); i!=assigned->end(); ++i) {
        Var * asgn_var = csp->vars[i->first];
        if(asgn_var->cage_idx == var->cage_idx){
            cage_vals.push_back(i->second);
            // cout << ' ' << asgn_var->idx << ":"
            //      << i->second << " ";
        }
    }
    
    
    // cout << "  **  ";
    
    if(cage_vals.size() == cage->vars.size()){
        if(cage->op == OP_ADD){
            short target = 0;
            for(short i=0; i<cage_vals.size(); ++i) target += cage_vals[i];
            if(target != cage->val){
                // cout << "CageTargetVal test failed " << target
                //      << "!=" << cage->val << " (ADD)" << endl;
                return false;
            }
        }
        
        if(cage->op == OP_SUB){
            sort(cage_vals.begin(), cage_vals.end());
            reverse(cage_vals.begin(), cage_vals.end());
            short target = cage_vals[0];
            for(short i=1; i<cage_vals.size(); ++i) target -= cage_vals[i];
            if(target != cage->val){
                // cout << "CageTargetVal test failed " << target
                //      << "!=" << cage->val << " (SUB)" << endl;
                return false;
            }
        }
        if(cage->op == OP_MUL){
            short target = 1;
            for(short i=0; i<cage_vals.size(); ++i) target *= cage_vals[i];
            if(target != cage->val){
                // cout << "CageTargetVal test failed " << target
                //      << "!=" << cage->val << " (MUL)" << endl;
                return false;
            }
        }
        if(cage->op == OP_DIV){
            sort(cage_vals.begin(), cage_vals.end());
            reverse(cage_vals.begin(), cage_vals.end());
            short target = cage_vals[0];
            for(short i=1; i<cage_vals.size(); ++i) target /= cage_vals[i];
            if(target != cage->val) {
            // cout << "CageTargetVal test failed " << target
            //      << "!=" << cage->val << " (DIV)" << endl;
                return false;
            }
        }
    }
    
    
    // cout << "test passed" << endl;
    return true;
    
}


short
mrv_selection(set<short> * unassigned, CSP * csp, map<short, short> * assigned) {
    short selected_id = *(unassigned->begin());
    short selected_rv = csp->psize;
    short rv;
    
    for(set<short>::const_iterator i=unassigned->begin(); i!=unassigned->end(); ++i){
        Var * var = csp->vars[*i];
        rv = 0;
        
        for(set<short>::const_iterator j=var->dom.begin(); j!=var->dom.end(); ++j)
            if(consistent(*j, *i, assigned, csp))
                ++ rv;
                
        if(rv < selected_rv){
            selected_rv = rv;
            selected_id = *i;
        }
    }
    
    return selected_id;
}

short
md_selection(set<short> * unassigned, CSP * csp, map<short, short> * assigned){
    short sx = 0;
    short sy = 0;
    short md = 0;
    short ** MD_CACHE = csp->MD_CACHE;
    for(short i=0;i<csp->psize;++i)
        for(short j=0;j<csp->psize;++j)
            MD_CACHE[i][j] = 0;
    for(map<short, short>::const_iterator i=assigned->begin(); i!=assigned->end(); ++i){
        Var * var = csp->vars[i->first];
        for(short j=0; j<csp->psize; ++j){
            ++ MD_CACHE[var->x][j];
            ++ MD_CACHE[j][var->y];
        }
        MD_CACHE[var->x][var->y] = -128;
    }
    for(short i=0;i<csp->psize;++i)
        for(short j=0;j<csp->psize;++j)
            if(MD_CACHE[i][j] > md){
                md = MD_CACHE[i][j];
                sx = i;
                sy = j;
            }
    return csp->vmap[sx][sy]->idx;
}

int traces = 0;

map<short, short> *
backtrack(map<short, short> * assigned, CSP * csp, set<short> * unassigned) {
    ++ traces;
    if(assigned->size() == csp->vars.size()){
        // cout << "backtrack:: Terminated" << endl;
        print_state(csp, assigned);
        return NULL;
    }
    
    
    // short var = naive_unassigned_selection(unassigned);
    // short var = md_selection(unassigned, csp, assigned);
    short var;
    if(assigned->size() > 8){
        var = mrv_selection(unassigned, csp, assigned);
    } else {
        var = md_selection(unassigned, csp, assigned);
    }
    
    set<short> * dom = order_domain_values(var, assigned, csp);
    
    for(set<short>::const_iterator i=dom->begin(); i!=dom->end(); ++i){
        short val = *i;
        if(consistent(val, var, assigned, csp)){
            assigned->insert(pair<short, short>(var, val));
            //cout << "added " << var << ":" << val << " to assigment" << endl;
            // cout << endl << "state:" << endl;
            // print_state(csp, assigned);
            // cout << endl << endl;
            unassigned->erase(var);
            map<short, short> * result = backtrack(assigned, csp, unassigned);
            if(result != NULL)
                return result;
            unassigned->insert(var);
            assigned->erase(var);
            //cout << "removed " << var << ':' << val << endl;
            //cout << endl;
        }
    }
    
    return NULL;
}


int
main(int argc, const char ** argv) {
    CSP * csp = init_problem(argv[1]);
    
    ofile.open(argv[2]);
    
    map<short, short> * empty_assigned = new map<short, short>();
    // cout << endl << endl;
    // print_state(csp, empty_assigned);
    // cout << endl;
    // prin_raw(csp);
    // cout << endl << endl;
    map<short, short> * assigned = reduce_domains(csp);
    set<short> * unassigned = init_unsaasigned(csp, assigned);
    // cout << endl << endl;
    // cout << "initial state:" << endl;
    // print_state(csp, assigned);
    // cout << endl << endl;
    map<short, short> * result  = backtrack(assigned, csp, unassigned);
    // cout << endl << endl;
    // print_state(csp, result);
    // cout << endl << endl;
    cout << endl << traces << endl;
    
    
    // ofile << "Writing this to a file.\n";
    
    ofile.close();
     
    return 0;
}