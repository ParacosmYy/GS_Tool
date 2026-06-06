#include "k36410/m36410.h"
QVector<double> m36410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
