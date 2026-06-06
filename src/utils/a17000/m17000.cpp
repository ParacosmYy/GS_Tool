#include "a17000/m17000.h"
QVector<double> m17000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
