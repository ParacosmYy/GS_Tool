#include "k32870/m32870.h"
QVector<double> m32870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
