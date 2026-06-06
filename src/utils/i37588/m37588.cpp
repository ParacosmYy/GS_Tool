#include "i37588/m37588.h"
QVector<double> m37588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
