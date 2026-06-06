#include "k29470/m29470.h"
QVector<double> m29470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
