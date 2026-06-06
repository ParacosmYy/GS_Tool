#include "g28126/m28126.h"
QVector<double> m28126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
