#include "j35029/m35029.h"
QVector<double> m35029::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
