#include "k25470/m25470.h"
QVector<double> m25470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
