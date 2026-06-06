#include "l18551/m18551.h"
QVector<double> m18551::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
