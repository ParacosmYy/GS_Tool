#include "k9470/m9470.h"
QVector<double> m9470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
