#include "q8016/m8016.h"
QVector<double> m8016::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
