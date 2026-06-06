#include "i28028/m28028.h"
QVector<double> m28028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
