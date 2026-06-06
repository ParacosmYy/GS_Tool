#include "i12508/m12508.h"
QVector<double> m12508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
