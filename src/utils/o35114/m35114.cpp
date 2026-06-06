#include "o35114/m35114.h"
QVector<double> m35114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
