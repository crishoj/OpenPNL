#include "pnlHigh.hpp"
#include "pnlDBN.hpp"   


int testDBNSmothing()
// this test provides smoothing with DBN
{
	int result = -1;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->EditEvidence("House-3^True Flat-3^False");
	// setting inference property: Smoothing
	pDBN->SetProperty("Inference","Smoothing");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-3 House-3");
	printf("%s",String(tmpJPD).c_str());
	// free memory
	delete pDBN;
	return result;
}

int testDBNFixLagSmothing()
// this test provides fixlagsmoothing with DBN
{
	int result = -1;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->EditEvidence("House-3^True Flat-3^False");
	// setting inference property:FixLagSmoothing
	pDBN->SetProperty("Inference","fix");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-0 House-0");
	printf("%s",String(tmpJPD).c_str());
	// free memory
	delete pDBN;
	return result;
}

int testDBNFiltering()
{
	int result = -1;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->EditEvidence("House-3^True Flat-3^False");
	// setting inference property:Filtering
	pDBN->SetProperty("Inference","filt");
	// getting request 
	TokArr  tmpJPD = pDBN->GetJPD("Street-0 Flat-0");
	printf("%s",String(tmpJPD).c_str());
	// free memory
	delete pDBN;
	return result;
}

int testDBNMPE()
{
	int result = -1;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences creation
	pDBN->EditEvidence("Street-0^True Flat-0^False");
	pDBN->EditEvidence("House-1^True Flat-1^False");
	pDBN->EditEvidence("Street-2^True Flat-2^False");
	pDBN->EditEvidence("House-3^True Flat-3^False");
	//  inference property must be Viterbi
	pDBN->SetProperty("Inference","viter");
	// getting request 
	TokArr  tmpMPE = pDBN->GetMPE("Street-1 Flat-1");
	printf("%s",String(tmpMPE).c_str());
	// free memory
	delete pDBN;
	return result;
} 

int testDBNLearning()
{
	int result = -1;
	// node values
	TokArr aChoice = "True False MayBe";
	// DBN creation
	DBN *pDBN; 
	pDBN = new DBN();
	// prior slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-0", aChoice); 
	pDBN->AddNode(discrete ^ "House-0", aChoice);
	pDBN->AddNode(discrete ^ "Flat-0", aChoice);
	// 1-st slice nodes creation 
	pDBN->AddNode(discrete ^ "Street-1", aChoice); 
	pDBN->AddNode(discrete ^ "House-1", aChoice);
	pDBN->AddNode(discrete ^ "Flat-1", aChoice);
	// edges creation
	pDBN->AddArc("Street-0","House-0");
	pDBN->AddArc("Street-0","Flat-0");
	pDBN->AddArc("Street-1"," House-1");
	pDBN->AddArc("Street-1","Flat-1");
	pDBN->AddArc("Street-0","Street-1"); // setting interface nodes
	// setting number of slices
	pDBN->SetNumSlices(4);
	//evidences generation
	pnl ::intVector nS(4);
	nS[0] = 6;
	nS[1] = 6;
	nS[2] = 6;
	nS[3] = 6;
	pDBN->GenerateEvidences(nS);
	// learning execution
	pDBN->LearnParameters();
	// free memory
	delete pDBN;
	return result;
}

int main(int ac, char **av)
{
	int res;
	res = testDBNSmothing();
	res = testDBNFixLagSmothing();
	res = testDBNFiltering();
	res = testDBNMPE();
	res = testDBNLearning();
	return res;
}


