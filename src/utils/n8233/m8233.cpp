#include "n8233/m8233.h"
QVector<double> m8233::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
