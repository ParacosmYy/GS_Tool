#include "a16860/m16860.h"
QVector<double> m16860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
