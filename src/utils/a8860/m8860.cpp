#include "a8860/m8860.h"
QVector<double> m8860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
