/*
authors: Joel Chacón Castillo, Carlos Segura González
*/
/*
  Differential Evolution with Diversity Replacement
           
  Version: 1.0   Date: 16/01/2018
*/

#include"de.h"
#include "bridge.h"

extern ClusteringBridge* g_clusteringBridge;

Fitness DIVERSITY::run() {
    // cout << max_num_evaluations<< endl;
  //cout << scientific << setprecision(8);
  initializeParameters();
  setSHADEParameters();
  //External variable...	
  ofstream outFitness;
  //Fitness....
  char fitnessName[500];
  strcpy(fitnessName, g_fileName);
  strcat(fitnessName, "_F");	
  outFitness.open(fitnessName, ios::out);
  //Dat
  //char varname[500];
  //strcpy(varName, g_fileName);
  //strcat(varname, "_dat");
  //outVar.pen(varName, ios::out);
    //results directory - create if not exists
    //string fileName_str(g_fileName);
    //size_t lastSlash = fileName_str.find_last_of('/');
    //if(lastSlash != string::npos){
    //    string directory = fileName_str.substr(0, lastSlash);
    //    mkdir(directory.c_str(), 0755);
    //}

  vector <Individual> pop;
  vector <Fitness> fitness(pop_size, 10);
  vector <Individual> children;
  vector <Fitness> children_fitness(pop_size, 10);
  vector <Individual> elite;
  vector <Fitness> elite_fitness(pop_size, 10);
  max_pop_size = pop_size;
  min_pop_size = 4;
  //Di = sqrt(problem_size)*0.1;

  //initialize pop, children and elite..
  for (int i = 0; i < pop_size; i++) {
        Individual newInd = makeNewIndividual();
        if(newInd == nullptr){
            return -1;
        }
        pop.push_back(newInd);
        Individual newChild = (variable*)malloc(sizeof(variable) * problem_size);
        if(newChild == nullptr){
            return -1;        
        }
        children.push_back(newChild);
        Individual newElite = (variable*)malloc(sizeof(variable) * problem_size);
        if(newElite == nullptr){
            return -1;
        }
        elite.push_back(newElite);
  }
  if(g_clusteringBridge !=  nullptr){
    g_clusteringBridge->getNumClusters();
    g_clusteringBridge->getNumPoints();
  }
  evaluatePopulation(pop, fitness);
  for(int i = 0; i < pop_size; i++)
  {
	elite_fitness[i] = fitness[i];
	for(int j = 0; j < problem_size; j++) // copy the parents information to elite...
	elite[i][j] = pop[i][j];
  }
  evaluatePopulation(elite, elite_fitness);


  //initialize and update the memory of the best individual

  Individual bsf_solution = (variable*)malloc(sizeof(variable) * problem_size);
  Fitness bsf_fitness;

  if ((fitness[0] - optimum) < epsilon) fitness[0] = optimum;
  bsf_fitness = fitness[0];
  for (int j = 0; j < problem_size; j ++) bsf_solution[j] = pop[0][j];

  for (int i = 0; i < pop_size; i++) {
    nfes++;

    //if ((fitness[i] - optimum) < epsilon) fitness[i] = optimum;


    if (fitness[i] < bsf_fitness) {
      bsf_fitness = fitness[i];
      for (int j = 0; j < problem_size; j ++) bsf_solution[j] = pop[i][j];
    }


    if (nfes >= max_num_evaluations) break;
  }
  ////////////////////////////////////////////////////////////////////////////


  variable *pop_sf = (variable*)malloc(sizeof(variable) * pop_size);
  variable *pop_cr = (variable*)malloc(sizeof(variable) * pop_size);

  //for current-to-pbest/1 --- temporally deactive
  //int p_best_ind;
  //int p_num = round(pop_size *  p_best_rate);
  //int *sorted_array = (int*)malloc(sizeof(int) * pop_size);
  //Fitness *temp_fit = (Fitness*)malloc(sizeof(Fitness) * pop_size);

  //main loop
  while (nfes < max_num_evaluations) 
  { 
      //Decrement the linear parameter of diversity
      updateD();
     // for (int i = 0; i < pop_size; i++) sorted_array[i] = i;
     // for (int i = 0; i < pop_size; i++) temp_fit[i] = elite_fitness[i];
     // sortIndexWithQuickSort(&temp_fit[0], 0, pop_size - 1, sorted_array);
     
      for (int target = 0; target < pop_size; target++) 
      {
        //generate random CR and F params
          if(randDouble() > 0.5)
            pop_cr[target] =  gauss(0.1, 0.1 );
          else
            pop_cr[target] =  gauss(0.9, 0.1 );

          if (pop_cr[target] > 1) pop_cr[target] = 1;
          else if (pop_cr[target] < 0) pop_cr[target] = 0;	

        //generate F_i and repair its value
        do {	
          pop_sf[target] = cauchy_g(0.5, 0.5*(double)nfes/max_num_evaluations );
         // pop_sf[target] = cauchy_g(0.5, 0.5);
        } while (pop_sf[target] <= 0.0);

        if (pop_sf[target] > 1) pop_sf[target] = 1.0;

        //it's for the mutation p-best...
        //do {
        //   p_best_ind = sorted_array[rand() % p_num];
        //} while (p_best_ind == target);                  
        operateCurrentTo1Bin(pop, &children[target][0], target, pop_sf[target], pop_cr[target]);
      }
  
    // evaluate the children's fitness values
    evaluatePopulation(children, children_fitness);

    /////////////////////////////////////////////////////////////////////////
    //update the bsf-solution and check the current number of fitness evaluations
    // if the current number of fitness evaluations over the max number of fitness evaluations, the search is terminated
    // the optimum is setted if the error is below of 1e-08
    for (int i = 0; i < pop_size; i++) 
     {
        nfes++;

       //if ((elite_fitness[i] - optimum) < epsilon) elite_fitness[i] = optimum;

        if (elite_fitness[i] < bsf_fitness) 
        {
          bsf_fitness = elite_fitness[i];
          for (int j = 0; j < problem_size; j ++) bsf_solution[j] = elite[i][j];
        }

        if (nfes >= max_num_evaluations) break;
     }
    
    // Differential evolution selection
    for (int i = 0; i < pop_size; i++) 
    {
      if (children_fitness[i] == fitness[i]) 
      {
  	elite_fitness[i] = children_fitness[i];
  	for (int j = 0; j < problem_size; j ++) elite[i][j] = children[i][j];
      }
      else if (children_fitness[i] < elite_fitness[i]) 
      {
  	elite_fitness[i] = children_fitness[i];

  	for (int j = 0; j < problem_size; j ++) elite[i][j] = children[i][j];
      }

    }

	//double percent = 0.01;
	//static vector<bool> report( 1.0/percent + 5, false);
	/*for(double l = 0, z = 0 ; l < 1.0; l+=percent, z++)
	{
        if(  nfes > (int)(l*max_num_evaluations) && !report[z] )
	    {
	        for(int i = 0; i < pop_size; i++)
		{
            
			//print the population information.....
			for(int k = 0 ; k < problem_size; k++)
			  //cout << pop[i][k] << " ";
			  outVar << pop[i][k] << " ";
			for(int k = 0 ; k < problem_size; k++)
			  outVar << children[i][k] << " ";
			for(int k = 0 ; k < problem_size; k++)
			  outVar << elite[i][k] << " ";
			outVar << endl;
			//cout<< endl;
			report[z] = true;
		}		
        outFitness << bsf_fitness<< " ";
		for(int k = 0 ; k < problem_size; k++){
		 outFitness << bsf_solution[k] << " ";
		}
        outFitness <<endl;
	   }
	}*/
	
     replacement(pop, fitness, children, children_fitness, elite, elite_fitness);
	
    //reducePopulation(pop, fitness, elite, elite_fitness, nfes, max_num_evaluations);
  }


 //free dynamic memory 
  for (int i = 0; i < pop_size; i++) 
  {

	//print the last population information.....
		/*for(int k = 0 ; k < problem_size; k++)
		  outVar << pop[i][k] << " ";
		for(int k = 0 ; k < problem_size; k++)
		  outVar << children[i][k] << " ";
		for(int k = 0 ; k < problem_size; k++)
		  outVar << elite[i][k] << " ";
		outVar << endl;*/
		
	    if(pop[i]) free(pop[i]); 
	    if(children[i]) free(children[i]); 
	    if(elite[i]) free(elite[i]);
  
  }
  
    if(outFitness.is_open()){
        outFitness << bsf_fitness << " ";
        for(int k = 0; k < problem_size; k++){
            outFitness << bsf_solution[k] << " ";
        }
        outFitness.close();
    }

    pop.clear(); 
    children.clear(); 
    fitness.clear();
    elite.clear();
    children_fitness.clear();
    free(bsf_solution);
    free(pop_sf);
    free(pop_cr);

  ////sorted_array.clear();
  //free(sorted_array);
  //free(temp_fit); //.clear();
    // cout << bsf_fitness << endl;
    return bsf_fitness - optimum;
}


void DIVERSITY::operateCurrentTo1Bin(const vector<Individual> &pop, Individual child, int &target, variable &scaling_factor, variable &cross_rate) {

  int r1, r2, r3;
  
  do {
    r1 = rand() % pop_size;
  } while (r1 == target);
  do {
    r2 = rand() % (pop_size);
  } while ((r2 == target) || (r2 == r1));
 do {
    r3 = rand() % (pop_size);
  } while ((r3 == target) || (r3 == r1) || (r2==r3));
  int random_variable = rand() % problem_size;

    for (int i = 0; i < problem_size; i++) 
    {
      if ((randDouble() < cross_rate) || (i == random_variable)) 
      {
	child[i] = pop[r3][i] + scaling_factor* (pop[r1][i] - pop[r2][i]);     // jSO
      }
      else 
	child[i] = pop[target][i];
    }
  //If the mutant vector violates bounds, the bound handling method is applied
  modifySolutionWithParentMedium(child,  pop[target]);
}

double DIVERSITY::distance(Individual A, Individual B)
{
   double distance = 0.0;
   for(int i = 0; i < problem_size; i++)
   {
	double term = (A[i] - B[i])/(max_region - min_region);
	distance += term*term;
  }
   return sqrt(distance);
}

void DIVERSITY::updateD()
{
	double TElapsed = nfes;
        double TEnd = max_num_evaluations;

        D = Di - Di * (1.1*TElapsed / TEnd  );
	
	D = ( D<=0 )? 0: D ;
	if( nfes > (double)max_num_evaluations/1.1) D = -1;
	else if( nfes > (double)max_num_evaluations/1.05) D = -1;
}

void DIVERSITY::replacement(vector<Individual> &pop, vector<Fitness> &fitness, vector<Individual> &children,vector<Fitness> &children_fitness, vector<Individual> &elite, vector<Fitness> &elite_fitness)
{
  priority_queue < pair <Fitness, Individual> > pq;
  vector<Individual> penalized;
  vector< vector<long double> > survivors;
  vector<Fitness> survivors_fitness, penalized_fitness;
  for(int i = 0 ; i < pop.size(); i++)
  {
	  pq.push( make_pair(-fitness[i], pop[i]));
	  pq.push( make_pair(-children_fitness[i], children[i] ));
	  pq.push( make_pair(-elite_fitness[i], elite[i]));
  }

  while(!pq.empty() &&  survivors.size() < pop.size())
  {
     pair<Fitness, Individual> data = pq.top();
     Individual current_individual = data.second;
     Fitness current_fitness = -data.first;
     pq.pop();
    
	if(survivors.size() >= pop.size())
	break;
	
	bool flag = true;

     for(int i = 0 ; i < survivors.size(); i++)
	{
	     double d =  distance(&survivors[i][0], current_individual) ;
		if(d < D)
		{
		   flag = false;
		   break;
		}
	}
	if(!flag)
	{
	   penalized_fitness.push_back(current_fitness);
	   penalized.push_back(current_individual);   
	}
	else
	{
	   survivors_fitness.push_back(current_fitness);
	   survivors.push_back(cpindividual(current_individual));
	}
  }

     vector<double> v_distances(penalized.size(), INFINITY);
     for(int i = 0 ;  i < penalized.size(); i++)
	{
	   for(int j = 0; j< survivors.size(); j++)
	   {
	      v_distances[i] = min(v_distances[i], distance( penalized[i], &survivors[j][0]));
	   }
	}

     while(survivors.size() < pop.size())
     {
	double maxd = -INFINITY;
	int maxindex = -1;

	//find the max near point from survivor to children....
        for(int i = 0 ; i < penalized.size(); i++)
	{
		if( v_distances[i] > maxd)
	  	{
			maxd = v_distances[i];
			maxindex = i;
	  	}
	}
	//update the modified values..
	
        for(int i = 0 ; i < penalized.size(); i++)
        {
	   if(i==maxindex) continue;
	   v_distances[i] = min(v_distances[i] , distance(penalized[maxindex], penalized[i] )  );
        }
	
        survivors.push_back(cpindividual(penalized[maxindex]));	
        survivors_fitness.push_back(penalized_fitness[maxindex]);	
	iter_swap(penalized.begin()+maxindex, penalized.end()-1);
	penalized.pop_back();
//	penalized.erase(penalized.begin()+maxindex);
	iter_swap(penalized_fitness.begin()+maxindex, penalized_fitness.end()-1);
	penalized_fitness.pop_back();
//	penalized_fitness.erase(penalized_fitness.begin()+maxindex);
	iter_swap(v_distances.begin()+maxindex, v_distances.end()-1);
	v_distances.pop_back();
     }

  for(int i = 0 ; i < pop.size(); i++)
  {
 	for(int j = 0 ; j < problem_size; j++)
	 pop[i][j] = survivors[i][j];
	
  }
	fitness = survivors_fitness;

}
void  DIVERSITY::setSHADEParameters() {
//  arc_rate = g_arc_rate;
 // arc_size = (int)round(pop_size * arc_rate);
 // p_best_rate = g_p_best_rate;
  //memory_size = g_memory_size;
   Di = g_Di;
}
void DIVERSITY::reducePopulation(vector<Individual> &pop, vector<Fitness> & fitness,vector<Individual> & elite, vector<Fitness> &elite_fitness, int nfes, int max_num_evaluations)
{

  if(D>0.0)return;
  double Start =  max_num_evaluations/1.1;

  double rate =  1.0 - (double)(nfes-Start)/(max_num_evaluations-Start);
  int reduction_ind_num = round( (max_pop_size - min_pop_size)*rate + min_pop_size );
 if (pop_size - reduction_ind_num <  min_pop_size) reduction_ind_num = pop_size - min_pop_size;

  int worst_ind_pop;
  int worst_ind_elite;

  for (int i = 0; i < reduction_ind_num; i++) 
  {
    worst_ind_pop = 0;
    worst_ind_elite = 0;
    for (int j = 1; j < pop_size; j++) {
      //if (fitness[j] > fitness[worst_ind]) worst_ind = j;
      if (fitness[j] > fitness[worst_ind_pop]) worst_ind_pop = j;
      if (elite_fitness[j] > elite_fitness[worst_ind_elite]) worst_ind_elite = j;
    }

    pop.erase(pop.begin() + worst_ind_pop);
    fitness.erase(fitness.begin() + worst_ind_pop);
    elite.erase(elite.begin() + worst_ind_elite);
    elite_fitness.erase(elite_fitness.begin() + worst_ind_elite);
    pop_size--;
  }
}
