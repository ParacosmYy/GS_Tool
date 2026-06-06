#include "a28600/m28600.h"
QVector<double> m28600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
