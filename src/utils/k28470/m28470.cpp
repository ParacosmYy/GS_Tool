#include "k28470/m28470.h"
QVector<double> m28470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
