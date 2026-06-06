#include "a19560/m19560.h"
QVector<double> m19560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
