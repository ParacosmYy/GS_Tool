#include "m28252/m28252.h"
QVector<double> m28252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
