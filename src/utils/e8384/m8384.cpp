#include "e8384/m8384.h"
QVector<double> m8384::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
