#include "i12028/m12028.h"
QVector<double> m12028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
