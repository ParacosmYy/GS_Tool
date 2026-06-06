#include "k9410/m9410.h"
QVector<double> m9410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
