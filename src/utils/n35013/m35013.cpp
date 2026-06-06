#include "n35013/m35013.h"
QVector<double> m35013::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
