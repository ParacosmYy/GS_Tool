#include "m28892/m28892.h"
QVector<double> m28892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
