/* 
Il diagramma dovrà modellare concetti astratti come Problem, State, Node, Action (che potranno essere istanziati dall'utente tramite sottoclassi concrete), e il concetto Frontier (con le diverse politiche di gestione). Una opportuna operazione dovrà modellare l'algoritmo generale di risoluzione (che sfrutterà la Frontier definita dall'utente per il suo problema). 
Potrebbe essere utile permettere all'utente (che definisce una sottoclasse di Problem) di specificare alcune caratteristiche note dello spazio degli stati, se queste possono essere utili all'algoritmo generico (ad es., il fatto che il modello di transizione è in realtà un albero, cosa che permetterebbe di evitare di ricordare gli stati visitati).
*/

template <typename T, typename C>
class Frontier {
    protected:
        C structure;

    public:
        void add(T item) {
            structure.push(item);
        }

        T get() {
            T item;
            // distinguishes between structures that have the "front" method (queue)
            // and the others (which use "top")
            if constexpr (requires { structure.front(); }) { 
                item = structure.front();
            } else item = structure.top();

            structure.pop();
            return item;
        }
};



