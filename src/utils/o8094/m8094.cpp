#include "o8094/m8094.h"
QVector<double> m8094::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
