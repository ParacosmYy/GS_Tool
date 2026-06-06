#include "m30152/m30152.h"
QVector<double> m30152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
