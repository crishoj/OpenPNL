%generate samples from Water Sprinkler network
testBNet = WaterSprinklerBNetCreation;
nsamples = 200;
evidences = GenerateSamples(testBNet, nsamples);

%bayesian network reconstruction  

%create an empty graph with the same number of nodes
nnodes = GetNumberOfNodes(testBNet);
mat = zeros(nnodes);
graph = CGraphCreateFromAdjMat(mat);

%create BNet with the same model domain
MD = GetModelDomain(testBNet);
bnet = CBNetCreateWithRandomMatrices(graph, MD);

%create learning engine
eng = CBicLearningEngineCreate(bnet);
%set input data
SetData(eng, evidences);
%startlearning
Learn(eng);

%get result of learning
resBNet = GetGraphicalModel(eng);
resGraph = GetGraph(resBNet);
resAdjMat = CreateAdjacencyMatrix(resGraph);

testGraph = GetGraph(testBNet);
testAdjMat = CreateAdjacencyMatrix(testGraph);

disp('make a comparison')

disp('Adjacency matrix of the test bnet');
disp(testAdjMat);

disp('Adjacency matrix of the result bnet');
disp(resAdjMat);



