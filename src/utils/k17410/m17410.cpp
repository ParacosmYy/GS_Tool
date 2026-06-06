#include "k17410/m17410.h"
QVector<double> m17410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
