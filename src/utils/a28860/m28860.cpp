#include "a28860/m28860.h"
QVector<double> m28860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
