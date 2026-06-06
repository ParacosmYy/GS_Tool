#include "c8262/m8262.h"
QVector<double> m8262::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
