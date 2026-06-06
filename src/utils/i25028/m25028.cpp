#include "i25028/m25028.h"
QVector<double> m25028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
