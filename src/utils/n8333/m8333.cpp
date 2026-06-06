#include "n8333/m8333.h"
QVector<double> m8333::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
