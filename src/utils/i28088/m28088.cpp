#include "i28088/m28088.h"
QVector<double> m28088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
