#include "k16410/m16410.h"
QVector<double> m16410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
