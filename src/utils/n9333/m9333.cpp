#include "n9333/m9333.h"
QVector<double> m9333::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
