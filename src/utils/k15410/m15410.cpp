#include "k15410/m15410.h"
QVector<double> m15410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
