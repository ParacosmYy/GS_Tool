#include "f37645/m37645.h"
QVector<double> m37645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
