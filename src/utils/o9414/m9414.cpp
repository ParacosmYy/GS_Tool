#include "o9414/m9414.h"
QVector<double> m9414::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
