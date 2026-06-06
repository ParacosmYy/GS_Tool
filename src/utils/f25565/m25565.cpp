#include "f25565/m25565.h"
QVector<double> m25565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
