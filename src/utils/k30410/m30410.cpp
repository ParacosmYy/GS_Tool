#include "k30410/m30410.h"
QVector<double> m30410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
