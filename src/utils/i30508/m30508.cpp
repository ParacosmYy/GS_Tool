#include "i30508/m30508.h"
QVector<double> m30508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
