#include "c18262/m18262.h"
QVector<double> m18262::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
