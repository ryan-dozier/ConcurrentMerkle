Implementation of a Concurrent Merkle Tree

Todo List:
* Test insert/delete when using multiple threads
* Work on what the verify() function will look like
* Look at state of the art binary search trees

On a small scale sequential test the root hash matches what would be expected by performing the hashed individually. 

> Things to work on before Monday 
>
> In our discussion, I would like to clarify the scope of your project - what are the key properties we want to implement/achieve, what are the key novelties (and what are the top 3 most closely related papers), and do we have some specific use cases/applications in mind for this data structure. I just mention these questions here so that you can think about them and discuss when we meet.

Scope/Properties
* high performance on insert/delete/contains. These will be the most common operations when using the tree.
* the novelty of this project/paper will be that there are no concurrent merkle trees in existance other than coarse grained versions.
  * during the senior design project we stumbled upon a "fine-grained" solution that students at UC Berkely came up with. However their version is only fine grained when multiple items are batch inserted. 
    * in the case where two threads independently submit a single item the tree is locked from the root (coarse-grained)

Related Papers:
* This is a problem area for the project as there really is no literature regarding concurrent merkle trees other than the fine grained student written paper.
  * We can however cite some ways in which we are making it concurrent. I believe I will need to use descriptor objects when modifying the leaf nodes, I was hoping to avoid this but after testing I think it may be necessary.
  * For comparisons I think it will be necessary to compare with a coarse-grained solution, and potentially an STM based aproach. Because most uses currently use coarse-grained implementations i.e. ethereum/bitcoin/intel/google it would show the performance difference to the "standard"
* I did find some recent papers on uses for merkle hashes. the related work may end up being a combination of uses/why merkle trees matter, and then cite different concurrency techniques used in the creation (descriptors ect.)
  * concurrent query authentication (Sep 2019)
    * IEEE Transactions on Knowledge and Data Engineering
  * remote data authentication (Dec 2019)
    * Communications in Computer and Information Science (CCIS, volume 1137)


Specific Use Cases:
* Potentially intel secure file i/o, this will depend on thread safety when accessing the same file, but currently their "merkle-esque" tree is used in a sequential manner
* Apache Cassandra/ Amazon DynamoDB both use merkle trees. I haven't dived too deep into how they are utilized. For DynamoDB it looked like they were using a merkle tree to help recover from crashes. If this is the case, it would stand to reason that having a concurrent version would allow them to more efficiently load information into the data structure.
* Public key exchange (google Trillian)
