#include "a32860/m32860.h"
QVector<double> m32860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
