#include "a19000/m19000.h"
QVector<double> m19000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
