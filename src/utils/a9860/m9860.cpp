#include "a9860/m9860.h"
QVector<double> m9860::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
